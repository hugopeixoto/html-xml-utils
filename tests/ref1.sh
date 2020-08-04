:
trap 'rm $TMP1 $TMP2 $TMP3' 0
TMP1=`mktemp /tmp/tmp.XXXXXXXXXX` || exit 1
TMP2=`mktemp /tmp/tmp.XXXXXXXXXX` || exit 1
TMP3=`mktemp /tmp/tmp.XXXXXXXXXX` || exit 1

cat >$TMP1 <<-EOF
	<span>term 1</span>
	text...
	<dfn>term 1</dfn>
	EOF

# The echo adds a newline at the end of the file
#
(./hxref $TMP1; echo) >$TMP2

cat >$TMP3 <<-EOF
	<html><body><p><a href="#term-1">term 1</a>
	text...
	<dfn id="term-1">term 1</dfn>
	</p></body></html>
	EOF

cmp -s $TMP2 $TMP3
