:
trap 'rm $TMP1 $TMP2' 0
TMP1=`mktemp /tmp/tmp.XXXXXXXXXX` || exit 1
TMP2=`mktemp /tmp/tmp.XXXXXXXXXX` || exit 1

cat >$TMP1 <<-EOF
	<a href="foo/bar">.</a>
	<a href="http://example.org/">.</a>
	<a href="bar">.</a>
EOF
./hxcopy $TMP1 $TMP2
cmp -s $TMP1 $TMP2
