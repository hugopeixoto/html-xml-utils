:
trap 'rm $TMP1 $TMP2' 0
TMP1=`mktemp /tmp/tmp.XXXXXXXXXX` || exit 1
TMP2=`mktemp /tmp/tmp.XXXXXXXXXX` || exit 1

./hxtoc -d -t >$TMP1 <<-EOF
	<title>Test</title>
	<h1>Document heading</h1>
	<!--toc-->
	<section id=sec1>
	<p>Multiple headings in this section.
	<h1>Second-level heading</h1>
	<h3>Third-level heading</h3>
	<section id=sec2>
	<hgroup><h6>Another third-level heading</h6></hgroup>
	</section>
	</section>
EOF
echo >>$TMP1			# Add newline

cat >$TMP2 <<-EOF
<html><head><title>Test</title></head><body><h1 id="document-heading">Document heading</h1><!--begin-toc-->
<ul class="toc">
<li><a href="#document-heading">Document heading</a>
 <ul class="toc">
 <li><a href="#sec1">Second-level heading</a>
  <ul class="toc">
  <li><a href="#third-level-heading">Third-level heading</a>
  <li><a href="#sec2">Another third-level heading</a>
  </ul>
 </ul></ul>
<!--end-toc--><section id="sec1">
<p>Multiple headings in this section.
</p><h1>Second-level heading</h1>
<h3 id="third-level-heading">Third-level heading</h3>
<section id="sec2">
<hgroup><h6>Another third-level heading</h6></hgroup>
</section>
</section></body></html>
EOF

diff -u $TMP2 $TMP1
