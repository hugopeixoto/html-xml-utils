:
trap 'rm $TMP1 $TMP2' 0
TMP1=`mktemp /tmp/tmp.XXXXXXXXXX` || exit 1
TMP2=`mktemp /tmp/tmp.XXXXXXXXXX` || exit 1

cat >$TMP1 <<-EOF
	test 1
	test 2
EOF
./hxpipe file:$TMP1 | ./hxunpipe >$TMP2
cmp -s $TMP1 $TMP2
