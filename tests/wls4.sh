:
trap 'rm $TMP1 $TMP2 $TMP3' 0

TMP1=`mktemp /tmp/tmp.XXXXXXXXXX` || exit 1
TMP2=`mktemp /tmp/tmp.XXXXXXXXXX` || exit 1
TMP3=`mktemp /tmp/tmp.XXXXXXXXXX` || exit 1

cat >$TMP1 <<-EOF
	<a href="http://example.org/bar/foo&doesnotexist;.html">.</a>
	<a href="http://example.org/bar/foo&#;bar.html">.</a>
	<a href="http://example.org/bar/foo&">.</a>
	<a href="http://example.org/bar/foo&#-1;">.</a>
	<a href="http://8211.example.org/bar/foo&#8211;bar.html">.</a>
	<a href="http://ndash.example.org/bar/foo&ndash;bar.html">.</a>
	<a href="http://&dagger;.example.org/bar/foobar.html">.</a>
EOF
cat >$TMP2 <<-EOF
	http://example.org/bar/foo&doesnotexist;.html
	http://example.org/bar/foobar.html
	http://example.org/bar/foo&
	http://example.org/bar/foo
	http://8211.example.org/bar/foo–bar.html
	http://ndash.example.org/bar/foo–bar.html
	http://†.example.org/bar/foobar.html
EOF
./hxwls <$TMP1 >$TMP3
cmp -s $TMP2 $TMP3
