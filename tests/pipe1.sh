:
trap 'rm $TMP1 $TMP2' 0
TMP1=`mktemp /tmp/tmp.XXXXXXXXXX` || exit 1
TMP2=`mktemp /tmp/tmp.XXXXXXXXXX` || exit 1

./hxpipe -l >$TMP1 <<-EOF
	<style>/*<![CDATA[*/
	<xyz>
	/*]]>*/</style>
EOF
cat >$TMP2 <<-EOF
	L1
	(style
	L1
	-/*<![CDATA[*/\n<xyz>\n/*]]>*/
	L3
	)style
	L4
	-\n
EOF

cmp -s $TMP1 $TMP2
