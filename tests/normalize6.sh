:
trap 'rm $TMP1 $TMP2 $TMP3' 0
TMP1=`mktemp /tmp/tmp.XXXXXXXXXX` || exit 1
TMP2=`mktemp /tmp/tmp.XXXXXXXXXX` || exit 1
TMP3=`mktemp /tmp/tmp.XXXXXXXXXX` || exit 1

cat >$TMP1 <<-EOF
	<!DOCTYPE html>

	<html lang="en">
	<head>
	<style>
	/* no <elements> or <![CDATA[ mark-up here */
	</style>
	</head>
	  <p>...</p>
	</html>
EOF
cat >$TMP2 <<-EOF
	<!DOCTYPE html>

	<html lang=en>
	<head>
	<style>
	/* no <elements> or <![CDATA[ mark-up here */
	</style>

	<body>
	<p>...
EOF
./hxnormalize -x $TMP1 | ./hxnormalize -i 0 -X >$TMP3
diff -u $TMP2 $TMP3
