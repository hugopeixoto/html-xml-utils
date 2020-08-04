:
trap 'rm $TMP1 $TMP2' 0
TMP1=`mktemp /tmp/tmp.XXXXXXXXXX` || exit 1
TMP2=`mktemp /tmp/tmp.XXXXXXXXXX` || exit 1

./hxuncdata >$TMP1 <<-EOF
	<style>/*<![CDATA[*/
	<xyz>
	/*]]>*/</style>
EOF
cat >$TMP2 <<-EOF
	<style>/**/
	&lt;xyz&gt;
	/**/</style>
EOF

# if cmp -s $TMP1 $TMP2; then echo Pass; else echo Fail; exit 2; fi
cmp -s $TMP1 $TMP2
