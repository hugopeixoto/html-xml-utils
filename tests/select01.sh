:

trap 'rm $TMP1 $TMP2' 0
TMP1=`mktemp /tmp/addidXXXXXXXX` || exit 1
TMP2=`mktemp /tmp/addidXXXXXXXX` || exit 1

./hxselect p >$TMP1 <<-EOF
	<div>
	<p>Select this.</p>
	<address>But not this.</address>
	</div>
	<p>Select this, too.</p>
EOF

# Add a newline
echo >>$TMP1

cat >$TMP2 <<-EOF
	<p>Select this.</p><p>Select this, too.</p>
EOF

diff -u $TMP1 $TMP2
# cmp -s $TMP1 $TMP2
