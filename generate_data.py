import random
import time

def generate_csv(filename, num_orders=1000000):
    with open(filename, 'w') as f:
        f.write("timestamp,side,price,quantity,type\n")
        start_ts = int(time.time() * 1e9)
        
        for i in range(num_orders):
            ts = start_ts + i * 100 # 100ns apart
            side = 'B' if random.random() > 0.5 else 'S'
            # Generate prices around 100.00 with 4 decimal places (fixed point * 10000)
            price = random.randint(990000, 1010000)
            qty = random.randint(1, 100)
            order_type = 'L' # Limit order
            
            f.write(f"{ts},{side},{price},{qty},{order_type}\n")

if __name__ == "__main__":
    print("Generating 1,000,000 synthetic orders...")
    generate_csv("orders.csv", 1000000)
    print("Done. File: orders.csv")
