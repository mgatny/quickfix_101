#!/bin/sh

rm -rf *.cache config
mkdir config

touch README.md NEWS AUTHORS INSTALL ChangeLog
[ -L README ] || ln -s README.md README

set -e
autoreconf -f -i -Wall,no-obsolete
rm -rf autom4te.cache config.h.in~

