:
trap 'rm $TMP1 $TMP2' 0
TMP1=`mktemp /tmp/tmp.XXXXXXXXXX` || exit 1
TMP2=`mktemp /tmp/tmp.XXXXXXXXXX` || exit 1

./asc2xml <<-EOF | ./xml2asc >$TMP1
	abc123
	&#160;&#161;&#162;
	&#8220; &#8222; &#8222;
	&#x2600;&#x2601;&#x2602;
	&#9;&#x2603;
EOF
cat >$TMP2 <<-EOF
	abc123
	&#160;&#161;&#162;
	&#8220; &#8222; &#8222;
	&#9728;&#9729;&#9730;
	&#9;&#9731;
EOF
	
cmp -s $TMP1 $TMP2
