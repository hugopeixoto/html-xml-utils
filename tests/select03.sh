:

trap 'rm $TMP1 $TMP2' 0
TMP1=`mktemp /tmp/addidXXXXXXXX` || exit 1
TMP2=`mktemp /tmp/addidXXXXXXXX` || exit 1

./hxselect -s '\n' -c p >$TMP1 <<-EOF
	<div>
	<p>Select this.</p>
	<address>But not this.</address>
	</div>
	<p>Select this, too.</p>
EOF
cat >$TMP2 <<-EOF
	Select this.
	Select this, too.
EOF

cmp -s $TMP1 $TMP2
