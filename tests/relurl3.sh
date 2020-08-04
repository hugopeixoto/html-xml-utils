:
trap 'rm $TMP1 $TMP2' 0
TMP1=`mktemp /tmp/tmp.XXXXXXXXXX` || exit 1
TMP2=`mktemp /tmp/tmp.XXXXXXXXXX` || exit 1

./hxwls -l -b "http://example.org/remove/this" >$TMP1 <<-EOF
	<link rel=rel href="http:///path"><!-- empty host -->
EOF
cat >$TMP2 <<-EOF
	link	rel	http:/path
EOF

cmp -s $TMP1 $TMP2
