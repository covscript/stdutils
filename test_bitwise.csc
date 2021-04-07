import bitwise
using  bitwise
context.add_literal("hex", hex_literal)
@begin
system.out.println(
    "0x1"hex
        .shift_left(3)
        .logic_and("0xA"hex)
        .any()
)
@end