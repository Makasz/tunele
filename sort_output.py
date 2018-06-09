file = open('log.txt', 'r', encoding='utf8')
t = []
for row in file:
    splited = row.split(' ')
    if 'L:' in splited[1]:
        t.append(splited)
t.sort(key=lambda x: (int(''.join([a for a in x[1] if a.isdigit()])), int(''.join([a for a in x[0] if a.isdigit()]))))
t = [' '.join(a) for a in t]
with open('log_fixed.txt', 'w', encoding='utf8') as output:
    for row in t:
        print(row)
        output.write(row)

