:
trap 'rm $TMP1 $TMP2 $TMP3' 0
TMP1=`mktemp /tmp/tmp.XXXXXXXXXX` || exit 1
TMP2=`mktemp /tmp/tmp.XXXXXXXXXX` || exit 1
TMP3=`mktemp /tmp/tmp.XXXXXXXXXX` || exit 1

cat >$TMP1 <<-EOF
	<!DOCTYPE html>

	<html>
	<body>
	<p title="ииииииииииииииииииииииииииииииииииииииииииииииииииииииииииииииииииииииииииииииии">ииииииии ииииииии ииииииии
EOF
cat >$TMP2 <<-EOF
	<!DOCTYPE html>

	<html>
	<body>
	<p
	title="ииииииииииииииииииииииииииииииииииииииииииииииииииииииииииииииииииииииииииииииии"
	>ииииииии ииииииии ииииииии
EOF
./hxnormalize -i 0 $TMP1 >$TMP3
cmp -s $TMP2 $TMP3
