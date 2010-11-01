#!/bin/sh
cp -r $1/build $2/Contents/Resources && (cd $2/Contents/Resources; find . | grep .svn$ | xargs rm -r )
cp -r $1/external/lib/macOS $2/Contents/Frameworks && (cd $2/Contents/Frameworks ; find . | grep .svn$ | xargs rm -r )
true
