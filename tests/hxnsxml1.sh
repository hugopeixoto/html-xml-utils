:
trap 'rm $TMP1 $TMP2 $TMP3 $TMP4 $TMP5' 0
TMP1=`mktemp /tmp/tmp.XXXXXXXXXX` || exit 1
TMP2=`mktemp /tmp/tmp.XXXXXXXXXX` || exit 1
TMP3=`mktemp /tmp/tmp.XXXXXXXXXX` || exit 1
TMP4=`mktemp /tmp/tmp.XXXXXXXXXX` || exit 1
TMP5=`mktemp /tmp/tmp.XXXXXXXXXX` || exit 1


cat >$TMP1 <<-EOF
	<a:b xmlns:a="xyz">
	<c xmlns:d="pqr" d:e="foo" d:f="bar" />
	</a:b>
EOF

# Running hxxmlns and hxnsxml twice should not change anything:
#
./hxxmlns $TMP1 >$TMP2
./hxnsxml $TMP2 >$TMP3
./hxxmlns $TMP3 >$TMP4
./hxnsxml $TMP4 >$TMP5

cmp -s $TMP2 $TMP4 && cmp -s $TMP3 $TMP5
