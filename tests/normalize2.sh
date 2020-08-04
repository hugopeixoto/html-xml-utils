:
trap 'rm $TMP1 $TMP2 $TMP3' 0
TMP1=`mktemp /tmp/tmp.XXXXXXXXXX` || exit 1
TMP2=`mktemp /tmp/tmp.XXXXXXXXXX` || exit 1
TMP3=`mktemp /tmp/tmp.XXXXXXXXXX` || exit 1

cat >$TMP1 <<-EOF
	<!DOCTYPE html>

	<html>
	<meta name=foo value=bar>
	<body>
	<dl>
	<dt>
	<meta name=foo value=baz>

EOF
cat >$TMP2 <<-EOF
	<!DOCTYPE html>

	<html>
	<head>
	<meta name=foo value=bar>

	<body>
	<dl>
	<dt>
	<meta name=foo value=baz>
	</dl>
EOF
./hxnormalize -i 0 -L $TMP1 >$TMP3
cmp -s $TMP2 $TMP3
