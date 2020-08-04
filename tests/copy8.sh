:
trap 'rm $TMP1 $TMP2 $TMP3' 0
TMP1=`mktemp /tmp/tmp.XXXXXXXXXX` || exit 1
TMP2=`mktemp /tmp/tmp.XXXXXXXXXX` || exit 1
TMP3=`mktemp /tmp/tmp.XXXXXXXXXX` || exit 1

cat >$TMP1 <<-EOF
	<a href="//example.org/foo">.</a>
	<a href="/foo">.</a>
EOF
cat >$TMP3 <<-EOF
	<a href="https://example.org/foo">.</a>
	<a href="https://example.org/foo">.</a>
EOF
./hxcopy -i https://example.org/foo1/bar -o ./ $TMP1 $TMP2

cmp -s $TMP2 $TMP3
