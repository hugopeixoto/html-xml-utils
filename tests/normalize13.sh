:
trap 'rm $TMP1 $TMP2 $TMP3' 0
TMP1=`mktemp /tmp/tmp.XXXXXXXXXX` || exit 1
TMP2=`mktemp /tmp/tmp.XXXXXXXXXX` || exit 1
TMP3=`mktemp /tmp/tmp.XXXXXXXXXX` || exit 1

cat >$TMP1 <<-EOF
	<!DOCTYPE html>

	<html>
	<head>
	<style>
	<![CDATA[123]]>
	&lt;foo>
	&amp;foo;&quot;&apos;&gt;
	</style>
	</head>
	<body>
	</body>
EOF
cat >$TMP2 <<-EOF
	<!DOCTYPE html>

	<html>
	<head>
	<style>
	123
	<foo>
	&foo;"'>
	</style>

	<body>
EOF
./hxnormalize -X -i 0 -L $TMP1 >$TMP3
diff -u $TMP2 $TMP3
