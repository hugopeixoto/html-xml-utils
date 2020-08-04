:

trap 'rm $TMP1 $TMP2' 0
TMP1=`mktemp /tmp/addidXXXXXXXX` || exit 1
TMP2=`mktemp /tmp/addidXXXXXXXX` || exit 1

./hxselect -c '\03B2' >$TMP1 <<-EOF
	<div>
	<β>Select this.</β>
	<address>But not this.</address>
	</div>
	<β>Select this, too.</β>
EOF
cat >$TMP2 <<-EOF
	Select this.Select this, too.
EOF

echo >>$TMP1			# Add newline
cmp -s $TMP1 $TMP2
