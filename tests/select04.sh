:

trap 'rm $TMP1 $TMP2' 0
TMP1=`mktemp /tmp/addidXXXXXXXX` || exit 1
TMP2=`mktemp /tmp/addidXXXXXXXX` || exit 1

./hxselect -s '\n' -c '::attr(title)' >$TMP1 <<-EOF
	<div>
	<p title="Select this.">Not this.</p>
	<address>And not this.</address>
	</div>
	<div title="Select this, too."/>
EOF
cat >$TMP2 <<-EOF
	Select this.
	Select this, too.
EOF

diff -u $TMP1 $TMP2
# cmp -s $TMP1 $TMP2
