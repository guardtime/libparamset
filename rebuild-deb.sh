#!/bin/bash

#
# Copyright 2013-2015 Guardtime, Inc.
#
# This file is part of the Guardtime client SDK.
#
# Licensed under the Apache License, Version 2.0 (the "License").
# You may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#     http://www.apache.org/licenses/LICENSE-2.0
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES, CONDITIONS, OR OTHER LICENSES OF ANY KIND, either
# express or implied. See the License for the specific language governing
# permissions and limitations under the License.
# "Guardtime" and "KSI" are trademarks or registered trademarks of
# Guardtime, Inc., and no license to trademarks is granted; Guardtime
# reserves and retains all trademark rights.
#

set -e

# Get version number.
VER=$(tr -d [:space:] < VERSION)
ARCH=$(dpkg --print-architecture)
PKG_VERSION=1


# Source directories.
include_dir=src/param_set
lib_dir=src/param_set/.libs
deb_dir=packaging/deb


lib_install_dir=usr/lib/
lib_doc_install_dir=usr/share/doc/param_set-$VER
dev_inc_install_dir=usr/include/param_set
dev_doc_install_dir=usr/share/doc/param_set-$VER
pconf_install_dir=usr/lib/pkgconfig


# Temporary directories for deb package build.
tmp_dir_lib=$deb_dir/tmp_lib
tmp_dir_dev=$deb_dir/tmp_dev
tmp_dir_src=$deb_dir/tmp_src


# File list for libparamset installion:
#   libparamset_libs - dynamic library for runtime, installed with libparamset package.
#   libparamset_changelog_license - changelog and license, installed with libparamset and libparamset-dev package.
#   libparamset_devel_libs - static libraries, installed with libparamset-dev package.
#   libparamset_dev_doc - doxygen documentation, installed with libparamset-dev package.
#   libparamset_pckg_config - package configuration file, installed with libparamset-dev package.
#   libparamset_devel_includes - include files, installed with libparamset-dev package.


libparamset_libs="\
	$lib_dir/libparamset.so \
	$lib_dir/libparamset.so.*"

libparamset_changelog_license="\
	changelog \
	LICENSE"

libparamset_devel_libs="\
	$lib_dir/libparamset.a \
	$lib_dir/../libparamset.la"

libparamset_dev_doc="\
	doc/html/"

libparamset_pckg_config="\
	libparamset.pc"

libparamset_devel_includes="\
	$include_dir/parameter.h \
	$include_dir/param_set.h \
	$include_dir/param_value.h \
	$include_dir/strn.h \
	$include_dir/task_def.h \
	$include_dir/wildcardexpanders.h \
	$include_dir/version.h \
	"


# Rebuild API.
./rebuild.sh
make dist


# Create temporary directory structure.
mkdir -p $tmp_dir_lib/libparamset/$lib_install_dir
mkdir -p $tmp_dir_lib/libparamset/$lib_doc_install_dir

mkdir -p $tmp_dir_dev/libparamset-dev/$lib_install_dir
mkdir -p $tmp_dir_dev/libparamset-dev/$pconf_install_dir
mkdir -p $tmp_dir_dev/libparamset-dev/$dev_inc_install_dir
mkdir -p $tmp_dir_dev/libparamset-dev/$dev_doc_install_dir

mkdir -p $tmp_dir_lib/libparamset/DEBIAN
mkdir -p $tmp_dir_dev/libparamset-dev/DEBIAN
mkdir -p $tmp_dir_src/libparamset/debian

chmod -Rf 755 $tmp_dir_lib
chmod -Rf 755 $tmp_dir_dev
chmod -Rf 755 $tmp_dir_src

# Copy control files and changelog.
cp  $deb_dir/libparamset/DEBIAN/control $tmp_dir_lib/libparamset/DEBIAN/control
cp  $deb_dir/libparamset/DEBIAN/control-devel $tmp_dir_dev/libparamset-dev/DEBIAN/control
cp  $deb_dir/libparamset/DEBIAN/control-source $tmp_dir_src/libparamset/debian/control
cp  $deb_dir/libparamset/DEBIAN/changelog $tmp_dir_src/libparamset/debian/

# As the target architecture do not match with the one provided by autotools,
# replace the variable by the one provided by dpkg.
sed -i s/@DPKG_VERSION_REPLACED_WITH_SED@/$ARCH/g "$tmp_dir_lib/libparamset/DEBIAN/control"
sed -i s/@DPKG_VERSION_REPLACED_WITH_SED@/$ARCH/g "$tmp_dir_dev/libparamset-dev/DEBIAN/control"


# Copy libparamset shared library with its changelog to target directories.
cp -fP $libparamset_libs $tmp_dir_lib/libparamset/$lib_install_dir/
cp -f $libparamset_changelog_license $tmp_dir_lib/libparamset/$lib_doc_install_dir/

# Copy libparamset static libraries, include files, (docygen documentation if is
# built) and package configuration file.
cp -fP $libparamset_devel_libs $tmp_dir_dev/libparamset-dev/$lib_install_dir/
cp -f $libparamset_devel_includes $tmp_dir_dev/libparamset-dev/$dev_inc_install_dir/
cp -f $libparamset_pckg_config $tmp_dir_dev/libparamset-dev/$pconf_install_dir/

# Rebuild doxygen documentation and copy files.
# Check if doxygen with supported version (>=1.8.0) is installed.
if (doxygen -v | grep -q -P -e '((^1\.([8-9]|[1-9][0-9]+))|(^[2-9]\.[0-9]+)|(^[0-9]{2,}\.[0-9]+))\.[0-9]+$') > /dev/null 2>&1 ; then
	make doc
	cp -f $libparamset_dev_doc $tmp_dir_dev/libparamset-dev/$dev_doc_install_dir/
else
	echo "Doxygen documentation not included into package!"
fi


cp -f libparamset-${VER}.tar.gz $tmp_dir_src/libparamset_${VER}.orig.tar.gz
cp  $deb_dir/libparamset/DEBIAN/changelog $tmp_dir_src/libparamset/debian/
tar -xvzf libparamset-${VER}.tar.gz -C $tmp_dir_src/
cp -r $tmp_dir_src/libparamset/debian $tmp_dir_src/libparamset-${VER}

# Build packages.
dpkg-deb --build $tmp_dir_lib/libparamset
mv $tmp_dir_lib/libparamset.deb libparamset_${VER}-${PKG_VERSION}_${ARCH}.deb

dpkg-deb --build $tmp_dir_dev/libparamset-dev
mv $tmp_dir_dev/libparamset-dev.deb libparamset-dev_${VER}-${PKG_VERSION}_${ARCH}.deb

dpkg-source -b -sn $tmp_dir_src/libparamset-${VER} ""


# Cleanup.

rm -rf $tmp_dir_lib
rm -rf $tmp_dir_dev
rm -rf $tmp_dir_src
rm libparamset-${VER}.tar.gz
