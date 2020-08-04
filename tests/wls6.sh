:
trap 'rm $TMP1 $TMP2 $TMP3' 0
TMP1=`mktemp /tmp/tmp.XXXXXXXXXX` || exit 1
TMP2=`mktemp /tmp/tmp.XXXXXXXXXX` || exit 1
TMP3=`mktemp /tmp/tmp.XXXXXXXXXX` || exit 1

cat >$TMP1 <<-EOF
	<link rel=stylesheet href="http://example.org/style.css">
	<a href="foo.html">.</a>
	<img src="bar/foo.png">
	<img src="../bar/foo.png">
	<img src="../bar/foo.png" srcset="../bar/foo.svg">
EOF
cat >$TMP2 <<-EOF
	link	stylesheet	http://example.org/style.css
	a		http://example.org/othersub/foo.html
	img		http://example.org/othersub/bar/foo.png
	img		http://example.org/bar/foo.png
	img		http://example.org/bar/foo.png
	img	srcset	http://example.org/bar/foo.svg
EOF
./hxwls -b http://example.org/othersub/base -l $TMP1 >$TMP3
diff -u $TMP2 $TMP3
