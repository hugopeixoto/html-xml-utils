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
	<picture>
	  <source type="image/svg+xml" srcset="foo.svg">
	  <img src="foo.png" alt="">
	</picture>
EOF
cat >$TMP2 <<-EOF
	http://example.org/style.css
	../othersub/foo.html
	../othersub/bar/foo.png
	../bar/foo.png
	../othersub/foo.svg
	../othersub/foo.png
EOF
./hxwls -b ../othersub/base $TMP1 >$TMP3
cmp -s $TMP2 $TMP3
