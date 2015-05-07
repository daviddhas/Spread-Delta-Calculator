import random

NUMLINES = 500

ids = [0, 1, 2]
levels = [0]
sides = [0, 1]
spreads = [2]

print("instrument_id,level,side,quantity,price")

for i in range(NUMLINES):

    instrument_id = random.choice(ids)
    side = random.choice(sides)
    level = random.choice(levels)

    quantity = random.randint(1, 100)
    price = random.randint(1, 5000)

    if instrument_id in spreads:
        price = random.randint(-5000, 5000)

    print("{},{},{},{},{}".format(instrument_id, level, side, quantity, price))
