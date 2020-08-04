:
trap 'rm -rf $TMP' 0
TMP=`mktemp -d /tmp/tmp.XXXXXXXXXX` || exit 1
EXEDIR=$PWD

cd -P $TMP || exit 1		# -P to avoid symlinks
cat >aaa <<-EOF
	<a href="foo/bar">.</a>
	<a href="http://example.org/">.</a>
	<a href="bar">.</a>
EOF

# Copy from relative path to absolute path
$EXEDIR/hxcopy aaa $PWD/bbb
cmp -s aaa bbb
