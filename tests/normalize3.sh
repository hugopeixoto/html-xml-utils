:
trap 'rm $TMP1 $TMP2 $TMP3' 0
TMP1=`mktemp /tmp/tmp.XXXXXXXXXX` || exit 1
TMP2=`mktemp /tmp/tmp.XXXXXXXXXX` || exit 1
TMP3=`mktemp /tmp/tmp.XXXXXXXXXX` || exit 1

cat >$TMP1 <<-EOF
	<!DOCTYPE html>

	<html>
	<style type="">
	Not a delimiter: <
	Not a delimiter: </
	Not a delimiter: </&
	Not a delimiter: </foo
	Not an entity: &
	</style>
	<body>
	<dl>
	<dt>

EOF
cat >$TMP2 <<-EOF
	<!DOCTYPE html>

	<html>
	 <head>
	  <style type="">
	Not a delimiter: <
	Not a delimiter: </
	Not a delimiter: </&
	Not a delimiter: </foo
	Not an entity: &
	</style>

	 <body>
	  <dl>
	   <dt>
	  </dl>
EOF
./hxnormalize -i 1 $TMP1 >$TMP3

cmp -s $TMP2 $TMP3
