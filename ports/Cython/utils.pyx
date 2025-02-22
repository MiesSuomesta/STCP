
def log_and_get_string_from_bytes(title, the_data):
    tmp = the_data
    if the_data is not None:
        print(f"{title}: // len: {len(the_data)} // {tmp} //")
    else:
        print(f"{title}: // len: nada // {tmp} //")
    return tmp

def log_and_get_bytes_from_string(title, the_data):
    tmp = the_data
    if the_data is not None:
        print(f"{title}: // len: {len(the_data)} // {tmp} //")
    else:
        print(f"{title}: // len: nada // {tmp} //")
    return tmp
