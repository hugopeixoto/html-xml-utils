:
trap 'rm $TMP1 $TMP2 $TMP3' 0
TMP1=`mktemp /tmp/tmp.XXXXXXXXXX` || exit 1
TMP2=`mktemp /tmp/tmp.XXXXXXXXXX` || exit 1
TMP3=`mktemp /tmp/tmp.XXXXXXXXXX` || exit 1

cat >$TMP1 <<-EOF
	<!DOCTYPE html>

	<html lang=en>
	<body>
	<h2>Heading</h2>
EOF
cat >$TMP2 <<-EOF
	<!DOCTYPE html>

	<html lang="en">
	<body>
	<h2><span class="secno">A) </span>Heading</h2>
EOF
./hxnum -2 '%n%A) ' $TMP1 >$TMP3
diff -u $TMP2 $TMP3
