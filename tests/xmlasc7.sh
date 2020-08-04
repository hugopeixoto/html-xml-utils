:
# Illegal UTF-8 sequence

echo -e "\0301\0277" | ./xml2asc >/dev/null
[[ $? != 0 ]]
