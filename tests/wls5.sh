:
if ! grep '#define HAVE_LIBIDN 1' config.h >/dev/null; then exit 77; fi

trap 'rm $TMP1 $TMP2 $TMP3' 0

TMP1=`mktemp /tmp/tmp.XXXXXXXXXX` || exit 1
TMP2=`mktemp /tmp/tmp.XXXXXXXXXX` || exit 1
TMP3=`mktemp /tmp/tmp.XXXXXXXXXX` || exit 1

cat >$TMP1 <<-EOF
	<a href="http://8211.example.org/bar/foo&#8211;bar.html">.</a>
	<a href="http://ndash.example.org/bar/foo&ndash;bar.html">.</a>
	<a href="http://caf&eacute;.example.org/bar/foobar.html">.</a>
EOF
cat >$TMP2 <<-EOF
	http://8211.example.org/bar/foo%E2%80%93bar.html
	http://ndash.example.org/bar/foo%E2%80%93bar.html
	http://xn--caf-dma.example.org/bar/foobar.html
EOF
./hxwls -a $TMP1 >$TMP3
cmp -s $TMP2 $TMP3
