import math

SAMPLE_COUNT = int(25 * 1.5)
SAMPLE_SCALE = 16
samples = []

SCALE = 7.5

def sigmoid(x):
    return 1 / (1 + math.exp((-SCALE * x) + (SCALE / 2)))

def sine(x):
    return 0.5 + ((math.sin((math.pi * x) - (math.pi / 2))) / 2)

def main():
    samples = [int(SAMPLE_SCALE * sigmoid(float(i) / SAMPLE_COUNT)) for i in range(SAMPLE_COUNT)]
    print("Sigmoid:", samples)
    samples = [int(SAMPLE_SCALE * sine(float(i) / SAMPLE_COUNT)) for i in range(SAMPLE_COUNT)]
    print("Sin:", samples)

if __name__ == "__main__":
    main()
