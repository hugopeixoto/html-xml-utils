:
trap 'rm $TMP1 $TMP2 $TMP3' 0
TMP1=`mktemp /tmp/tmp.XXXXXXXXXX` || exit 1
TMP2=`mktemp /tmp/tmp.XXXXXXXXXX` || exit 1
TMP3=`mktemp /tmp/tmp.XXXXXXXXXX` || exit 1

cat >$TMP1 <<-EOF
	<a href="foo/bar">.</a>
	<a href="http://example.org/">.</a>
	<a href="bar">.</a>
EOF
cat >$TMP3 <<-EOF
	<a href="http://example.com/foo1/foo/bar">.</a>
	<a href="http://example.org/">.</a>
	<a href="http://example.com/foo1/bar">.</a>
EOF
./hxcopy -i http://example.com/foo1/bar -o foo2/bar <$TMP1 >$TMP2
cmp -s $TMP2 $TMP3
