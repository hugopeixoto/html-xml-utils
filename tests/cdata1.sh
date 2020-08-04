:
trap 'rm $TMP1 $TMP2' 0
TMP1=`mktemp /tmp/tmp.XXXXXXXXXX` || exit 1
TMP2=`mktemp /tmp/tmp.XXXXXXXXXX` || exit 1

./hxnormalize -i 0 >$TMP1 <<-EOF
	<style>/*<![CDATA[*/
	<xyz>
	/*]]>*/</style>
EOF
cat >$TMP2 <<-EOF

	<html>
	<head>
	<style>/*<![CDATA[*/
	<xyz>
	/*]]>*/</style>
EOF

cmp -s $TMP1 $TMP2
