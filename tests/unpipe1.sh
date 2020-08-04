:
trap 'rm $TMP1 $TMP2' 0
TMP1=`mktemp /tmp/tmp.XXXXXXXXXX` || exit 1
TMP2=`mktemp /tmp/tmp.XXXXXXXXXX` || exit 1

./hxunpipe >$TMP1 <<-EOF
	!root "test1" test2
	-\n
	!root "" test2
	-\n
	!root "test1"
	-\n
	!root ""
	-\n
EOF
cat >$TMP2 <<-EOF
	<!DOCTYPE root PUBLIC "test1" "test2">
	<!DOCTYPE root SYSTEM "test2">
	<!DOCTYPE root PUBLIC "test1">
	<!DOCTYPE root>
EOF

cmp -s $TMP1 $TMP2
