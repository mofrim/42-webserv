import sys

def rot13(text: str) -> str:
    result = []
    for char in text:
        if 'a' <= char <= 'z':
            new_char = chr(((ord(char) - ord('a') + 13) % 26) + ord('a'))
        elif 'A' <= char <= 'Z':
            new_char = chr(((ord(char) - ord('A') + 13) % 26) + ord('A'))
        else:
            new_char = char
        result.append(new_char)
    return ''.join(result)


data: str = sys.stdin.read()
rot13Data: str = rot13(data).rstrip()

msg: str = "" 

msg += f"Content-Type: text/plain\r\n"
msg += f"Content-Length: {len(rot13Data)}\r\n"
msg += "\r\n"
msg += rot13Data

print(msg, end='')
