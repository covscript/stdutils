import sdk_extension as sdk

foreach i in range(10)
    var repl = sdk.repl.create({})
    repl.exec("var a = " + i)
    repl.exec("a")
    repl = null
end

system.out.println("Good")
