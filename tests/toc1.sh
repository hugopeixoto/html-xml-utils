:
trap 'rm $TMP1 $TMP2' 0
TMP1=`mktemp /tmp/tmp.XXXXXXXXXX` || exit 1
TMP2=`mktemp /tmp/tmp.XXXXXXXXXX` || exit 1

./hxtoc -d -t >$TMP1 <<-EOF
	<title>Test</title>
	<h1>Document heading</h1>
	<!--toc-->
	<section>
	<h1>Second-level heading</h1>
	<section>
	<h6>Third-level heading</h6>
	</section>
	</section>
EOF
echo >>$TMP1			# Add newline

cat >$TMP2 <<-EOF
<html><head><title>Test</title></head><body><h1 id="document-heading">Document heading</h1><!--begin-toc-->
<ul class="toc">
<li><a href="#document-heading">Document heading</a>
 <ul class="toc">
 <li><a href="#second-level-heading-third-level-heading">Second-level heading</a>
  <ul class="toc">
  <li><a href="#third-level-heading-">Third-level heading</a>
  </ul>
 </ul></ul>
<!--end-toc--><section id="second-level-heading-third-level-heading">
<h1>Second-level heading</h1>
<section id="third-level-heading-">
<h6>Third-level heading</h6>
</section>
</section></body></html>
EOF

cmp -s $TMP1 $TMP2
