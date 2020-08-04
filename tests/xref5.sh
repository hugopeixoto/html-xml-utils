:
trap 'rm $TMP1 $TMP2' 0
TMP1=`mktemp /tmp/tmp.XXXXXXXXXX` || exit 1
TMP2=`mktemp /tmp/tmp.XXXXXXXXXX` || exit 1

./hxref -l >$TMP1 <<-EOF
	<p lang=en><dfn>bounty</dfn> is referenced twice:
	<span class="copy-this">bounties</span> and <em title="bounty">here.</em>
EOF

# Add a newline at the end:
echo >>$TMP1

cat >$TMP2 <<-EOF
	<html><body><p lang="en"><dfn id="bounty">bounty</dfn> is referenced twice:
	<a href="#bounty" class="copy-this">bounties</a> and <a href="#bounty"><em title="bounty">here.</em></a>
	</p></body></html>
EOF

cmp -s $TMP1 $TMP2
