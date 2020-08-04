:
trap 'rm $TMP1 $TMP2' 0
TMP1=`mktemp /tmp/tmp.XXXXXXXXXX` || exit 1
TMP2=`mktemp /tmp/tmp.XXXXXXXXXX` || exit 1

./hxunent >$TMP1 <<-EOF
	&&unknown;&unknow &unkno;&unkn
EOF
cat >$TMP2 <<-EOF
	&&unknown;&unknow &unkno;&unkn
EOF

diff -u -a $TMP1 $TMP2
