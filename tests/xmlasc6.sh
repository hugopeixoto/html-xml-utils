:
# Illegal UTF-8 sequence (surrogate pair)

echo -e "\0355\0240\0200" | ./xml2asc >/dev/null
[[ $? != 0 ]]
