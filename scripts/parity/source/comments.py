from __future__ import annotations


def strip_comments(text: str) -> str:
    output: list[str] = []
    index = 0
    block = False
    line = False
    single = False
    double = False
    escaped = False
    while index < len(text):
        current = text[index]
        next_char = text[index + 1] if index + 1 < len(text) else ""
        if block:
            if current == "\n":
                output.append(current)
            block, index = (
                (False, index + 2)
                if current == "*" and next_char == "/"
                else (True, index + 1)
            )
            continue
        if line:
            line = current != "\n"
            if current == "\n":
                output.append(current)
            index += 1
            continue
        if escaped:
            output.append(current)
            escaped = False
            index += 1
            continue
        if (single or double) and current == "\\":
            output.append(current)
            escaped = True
            index += 1
            continue
        if not single and current == '"':
            double = not double
            output.append(current)
            index += 1
            continue
        if not double and current == "'":
            single = not single
            output.append(current)
            index += 1
            continue
        if not single and not double and current == "/" and next_char == "*":
            block = True
            index += 2
            continue
        if not single and not double and current == "/" and next_char == "/":
            line = True
            index += 2
            continue
        output.append(current)
        index += 1
    return "".join(output)
