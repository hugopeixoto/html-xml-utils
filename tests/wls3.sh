:
trap 'rm $TMP1 $TMP2 $TMP3' 0

TMP1=`mktemp /tmp/tmp.XXXXXXXXXX` || exit 1
TMP2=`mktemp /tmp/tmp.XXXXXXXXXX` || exit 1
TMP3=`mktemp /tmp/tmp.XXXXXXXXXX` || exit 1

cat >$TMP1 <<-EOF
	<a href="http://example.org/bar/foo&amp;bar.html">.</a>
	<a href="http://example.org/bar/foo&lt;bar.html">.</a>
	<a href="http://example.org/bar/foo&#65;bar.html">.</a>
	<a href="http://example.org/bar/foo&#x41;bar.html">.</a>
EOF
cat >$TMP2 <<-EOF
	http://example.org/bar/foo&bar.html
	http://example.org/bar/foo<bar.html
	http://example.org/bar/fooAbar.html
	http://example.org/bar/fooAbar.html
EOF
./hxwls $TMP1 >$TMP3
cmp -s $TMP2 $TMP3
