import re

def parse_comments_to_html(comments: str) -> str:
    """
    Parses multiple structured comments into an HTML documentation format.

    :param comments: The string containing all block comments.
    :return: HTML documentation as a string.
    """
    # Regex to extract individual comment blocks
    block_pattern = r"/\*([\s\S]*?)\*/"
    blocks = re.findall(block_pattern, comments)

    # Regex to extract tag-content pairs within a block
    tag_pattern = r"@(\w+)\s+(.*)"

    # Start building the HTML
    html = "<!DOCTYPE html>\n<html>\n<head>\n<title>Documentation</title>\n</head>\n<body>\n"
    html += "<h1>Documentation</h1>\n"

    for block in blocks:
        matches = re.findall(tag_pattern, block)
        if matches:
            html += "<div class='doc-block'>\n<ul>\n"
            for tag, content in matches:
                html += f"  <li><strong>{tag.capitalize()}:</strong> {content}</li>\n"
            html += "</ul>\n</div>\n"

    html += "</body>\n</html>"
    return html


# Read from file
file_name = "samba.h"
try:
    with open(file_name, "r") as file:
        source_code = file.read()

    # Generate HTML from comments in the file
    html_output = parse_comments_to_html(source_code)

    # Save the HTML output to a file
    output_file = "docs/documentation.html"
    with open(output_file, "w") as file:
        file.write(html_output)

    print(f"Documentation generated and saved to {output_file}.")
except FileNotFoundError:
    print(f"Error: File '{file_name}' not found.")
