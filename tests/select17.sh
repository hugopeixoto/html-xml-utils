:

trap 'rm $TMP1 $TMP2' 0
TMP1=`mktemp /tmp/addidXXXXXXXX` || exit 1
TMP2=`mktemp /tmp/addidXXXXXXXX` || exit 1

./hxselect -s '\n' 'div ::attr(title)' >$TMP1 <<-EOF
	<div>
	<p title="Select this.">Not this</p>
	<address>And not this.</address>
	</div>
	<div title="Not this either."/>
EOF
cat >$TMP2 <<-EOF
	title="Select this."
EOF

cmp -s $TMP1 $TMP2
