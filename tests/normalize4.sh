:
trap 'rm $TMP1 $TMP2 $TMP3' 0
TMP1=`mktemp /tmp/tmp.XXXXXXXXXX` || exit 1
TMP2=`mktemp /tmp/tmp.XXXXXXXXXX` || exit 1
TMP3=`mktemp /tmp/tmp.XXXXXXXXXX` || exit 1

cat >$TMP1 <<-EOF
	<!DOCTYPE html>

	<html>
	<script>//<![CDATA[
	Not a delimiter: <
	Not a delimiter: </
	Not a delimiter: </&
	Not a delimiter: </foo
	Not an entity: &
	No ] ] > needed, because this is a CDATA element.
	</script>
	<body>
	<p><![CDATA[<p></p>]]>

EOF
cat >$TMP2 <<-EOF
	<!DOCTYPE html>

	<html>
	  <body>
	    <script>//<![CDATA[
	Not a delimiter: <
	Not a delimiter: </
	Not a delimiter: </&
	Not a delimiter: </foo
	Not an entity: &
	No ] ] > needed, because this is a CDATA element.
	</script>

	  <body>
	    <p><![CDATA[<p></p>]]>
EOF
./hxnormalize -i 2 $TMP1 >$TMP3
cmp -s $TMP2 $TMP3
