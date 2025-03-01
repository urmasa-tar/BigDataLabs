matr = [['1', '2', '3', '4', '5', '6', '7', '8', '9'],
     ['1', '2', '3', '4', '5', '6', '7', '8', '9'],
     ['1', '2', '3', '4', '5', '6', '7', '.', '.'],
     ['1', '2', '3', '4', '5', '6', '7', '8', '9'],
     ['1', '2', '3', '4', '5', '6', '7', '8', '9'],
     ['1', '2', '3', '4', '5', '6', '7', '8', '9'],
     ['1', '2', '3', '4', '5', '6', '7', '8', '9'],
     ['1', '2', '3', '4', '5', '6', '7', '8', '9'],
     ['1', '2', '3', '4', '5', '6', '7', '8', '9'],
     ]
n = 9
res = True
'''
for row in range(n):
    matr.append([])
    matr = [int(i) for i in input().split()]
'''

for i in range(n):
    st_has = []
    for j in range(n):
        if(matr[i][j] in st_has):
            res = False
            break
        if(matr[i][j] != "."):
            st_has.append(matr[i][j])
    if(not(res)):
        break

for col in range(n):
    for row in range(n):
print(res)  