def networkkey_raw2pretty(nwkk: str):
    out = ""
    for i, c in enumerate(nwkk):
        out += c
        if i & 1:
            out += ':'

    if i & 1:
        return out[:-1]
    else:
        return out


if __name__ == "__main__":
    print(networkkey_raw2pretty("636f718d5b4a61d4672490104fc21ab4"))