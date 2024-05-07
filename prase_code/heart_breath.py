import ctypes

# 加载共享库
mylib = ctypes.CDLL('D:\Dev\seeeed-mmWave-demos\prase_project\libmmwave_heart.so')

class BaseStruct(ctypes.Structure):
    _pack_ = 1  # Set packing to 1 byte for alignment
    _fields_ = [
        ("sof", ctypes.c_uint8),
        ("id", ctypes.c_uint16),
        ("len", ctypes.c_uint16),
        ("type", ctypes.c_uint16),
        ("head_cksum", ctypes.c_uint8)
    ]

# Define HeartBreathPhase structure
class HeartBreathPhase(BaseStruct):
    _fields_ = [
        ("data", ctypes.c_uint32 * 3),
        ("data_cksum", ctypes.c_uint8)
    ]

# Define other structures similarly
class BreathRate(BaseStruct):
    _fields_ = [
        ("data", ctypes.c_uint32),
        ("data_cksum", ctypes.c_uint8)
    ]

class HeartBreathType(ctypes.c_uint16):
    HeartBreathPhaseType = 0x0A13
    BreathRateType = 0x0A14
    HeartRateType = 0x0A15
    HeartBreathDistanceType = 0x0A16

class Constructer(ctypes.Structure):
    _fields_ = [
        ("data_ptr", ctypes.c_void_p),
        ("type", HeartBreathType)
    ]

# 设置函数参数类型和返回值类型
# mylib.init_heart_breath.argtypes = [ctypes.POINTER(ctypes.c_uint8)]
mylib.init_heart_breath.restype = ctypes.POINTER(Constructer)

mylib.print_heart_breath.argtypes = [ctypes.POINTER(Constructer)]
mylib.print_heart_breath.restype = None

mylib.free_heart_breath.argtypes = [ctypes.POINTER(Constructer)]
mylib.free_heart_breath.restype = None


def test_one(hex_string:str):
    byte_data = bytes.fromhex(hex_string.replace(" ", ""))
    mylib.print_frame(byte_data, len(byte_data))

    result_ptr = mylib.init_heart_breath(byte_data)
    
    if result_ptr:
        mylib.print_heart_breath(result_ptr)
        mylib.free_heart_breath(result_ptr)
    else:
        # print("init_heart_breath returned NULL")
        pass
    

if __name__ == '__main__':
    hex_strings ="""01 42 7E 00 0C 0A 13 D7 45 9E 72 40 4E AE 4F 3F 42 57 28 3E 85 
    01 42 7F 00 08 0A 16 D7 
    01 00 00 00 AE 47 85 42 D0 
    01 42 80 00 04 0A 15 27 00 00 98 42 25 0D 0A 20 69 74 27 73 20 77 6F 72 6B 69 6E 67 21 21 21 21 20 0D 0A 
    01 42 81 00 0C 0A 13 28 8B 78 6D 40 B1 12 55 3F 27 AF 9E 3E C0 
    01 42 82 00 08 0A 16 2A 
    01 00 00 00 AE 47 85 42 D0 0D 0A 20 69 74 27 73 20 77 6F 72 6B 69 6E 67 21 21 21 21 20 0D 0A 
    01 42 83 00 0C 0A 13 2A 1D 1D 69 40 02 D8 59 3F 99 0F E0 3E 22 
    01 42 84 00 08 0A 16 2C 
    01 00 00 00 AE 47 85 42 D0 0D 0A 20 69 74 27 73 20 77 6F 72 6B 69 6E 67 21 21 21 21 20 0D 0A 
    01 42 85 00 0C 0A 13 2C D7 B8 64 40 67 F2 5D 3F 99 A7 09 3F 4B 
    01 42 86 00 08 0A 16 2E 
    01 00 00 00 AE 47 85 42 D0 0D 0A 20 69 74 27 73 20 77 6F 72 6B 69 6E 67 21 21 21 21 20 0D 0A 
    01 42 87 00 0C 0A 13 2E 4D 73 5F 40 EB 56 61 3F 72 06 1A 3F 6C 
    01 42 88 00 08 0A 16 20 
    01 00 00 00 AE 47 85 42 D0 0D 0A 20 69 74 27 73 20 77 6F 72 6B 69 6E 67 21 21 21 21 20 0D 0A 
    01 42 89 00 0C 0A 13 20 65 62 5A 40 42 FB 63 3F 04 89 1F 3F AA 
    01 42 8A 00 08 0A 16 22 
    """
    hex_strings.splitlines()
    hex_strings = [x.strip() for x in hex_strings.splitlines() if x.strip()]
    for i in hex_strings:
        test_one(i)
