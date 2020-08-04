:
trap 'rm $TMP1 $TMP2' 0
TMP1=`mktemp /tmp/tmp.XXXXXXXXXX` || exit 1
TMP2=`mktemp /tmp/tmp.XXXXXXXXXX` || exit 1


cat >$TMP1 <<-EOF
	<b>
	<c e="foo" f="bar" />
	</b>
EOF

# No namespaces in input then none in output
#
./hxnsxml $TMP1 >$TMP2

cmp -s $TMP1 $TMP2
