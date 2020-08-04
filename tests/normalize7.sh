:
trap 'rm $TMP1 $TMP2 $TMP3' 0
TMP1=`mktemp /tmp/tmp.XXXXXXXXXX` || exit 1
TMP2=`mktemp /tmp/tmp.XXXXXXXXXX` || exit 1
TMP3=`mktemp /tmp/tmp.XXXXXXXXXX` || exit 1

cat >$TMP1 <<-EOF
	<!DOCTYPE html>

	<html lang=en>
	<body>
	<p title="A very long attribute value that is longer than the line length and forces a line break after it">Text
EOF
cat >$TMP2 <<-EOF
	<!DOCTYPE html>

	<html lang=en>
	  <body>
	    <p
	      title="A very long attribute value that is longer than the line length and forces a line break after it"
	      >Text
EOF
./hxnormalize $TMP1 >$TMP3
cmp -s $TMP2 $TMP3
