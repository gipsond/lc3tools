import utils from '../utils'

describe('Assembler', function () {
    beforeEach(utils.beforeEach)
    afterEach(utils.afterEach)

    it('shows an error message when there is nothing to assemble', function () {
        return this.app.client.click('#build-button')
        .then(() => { return this.app.client.getHTML('#console', false) })
        .then(consoleContents => {
            consoleContents.should.equal('\
<span class="text-bold">\
<span class="text-red">\
error: \
</span>\
</span>\
<span class="text-bold">\
could not open  for reading\
</span>\
<br>'
            )
        })
    })

    describe('saves and assembles', function () {

        const successMessage = '\
<span class="text-bold">\
<span class="text-green">\
info: \
</span>\
</span>\
<span class="text-bold">\
attemping to assemble \'' + generatedFilePath + 'test.asm\'\
 into \'' + generatedFilePath + 'test.obj\'\
</span>\
<br>\
<span class="text-bold">\
<span class="text-green">\
info: \
</span>\
</span>\
<span class="text-bold">\
assembly successful\
</span>\
<br>'
        
        const saveAssembleAndAssertSuccess = function (client, lines) {
            // Start testing command by clicking text editor
            var command = client.click('div.ace_content')

            // For each line in the 'lines' parameter,
            // add a command to type that line and to type 'Enter'
            const typeLineCmds = lines.map(line => (() => { return client.keys(line) }))
            for (var typeLineCmd of typeLineCmds) {
                command = command.then(typeLineCmd)
                                 .then(() => { return client.keys('Enter') })
            }

            // Add commands to save and build,
            // then check console for success
            command = command.then(() => { return client.click('#save-button') })
                             .then(() => { return client.click('#build-button') })
                             .then(() => { return client.getHTML('#console', false) })
                             .then(consoleContents => {
                                 consoleContents.should.equal(successMessage)
                             })

            // Return generated command
            return command
        }

        it('the minimal LC-3 program', function () {
            return saveAssembleAndAssertSuccess(this.app.client,
                ['.ORIG x3000'
                , 'HALT'
                , '.END'
                ]
        )})

        it('an addition LC-3 program', function () {
            return saveAssembleAndAssertSuccess(this.app.client,
                ['.ORIG x4000'
                , 'ADD R0, R0, R0'
                , 'HALT'
                , '.END'
                ]
        )})

        it('the "Hello, World!" LC-3 program', function () {
            return saveAssembleAndAssertSuccess(this.app.client,
                ['.ORIG x3000'
                , 'LEA R0, GREETING'
                , 'PUTS'
                , 'HALT'
                , 'GREETING .STRINGZ "Hello, World!"'
                , '.END'
                ]
            )
        })

        it('a long, formatted LC-3 program', function () {
            this.timeout(5000)
            return saveAssembleAndAssertSuccess(this.app.client,
                ['\t.ORIG x3000' 
                , 'Back space'
                , '; Check Palindrome'
                , '; This program checks if the string at PTR is a palindrome.'
                , '\tLEA R0, PTR'
                , 'ADD R1, R0, #0'
                , 'Back space'
                , 'AGAIN LDR R2, R1, #0'
                , '\tBRz CONT'
                , 'ADD R1, R1, #1'
                , 'BRnzp AGAIN'
                , 'Back space'
                , 'CONT  ADD R1, R1, #-1'
                , 'Back space'
                , 'LOOP  LDR R3, R0, #0 ; Main Loop'
                , '\tLDR R4, R1, #0'
                , 'NOT R4, R4'
                , 'ADD R4, R4, #1'
                , 'ADD R3, R3, R4'
                , 'BRnp NO'
                , 'ADD R0, R0, #1'
                , 'ADD R1, R1, #-1'
                , 'NOT R2, R0'
                , 'ADD R2, R2, #1'
                , 'ADD R2, R1, R2'
                , 'BRnz YES'
                , 'BR LOOP'
                , 'Back space'
                , 'YES   AND R5, R5, #0'
                , '\tADD   R5, R5, #1'
                , 'BRnzp DONE'
                , 'Back space'
                , 'NO    AND R5, R5, #0'
                , 'DONE  HALT'
                , 'PTR   .FILL X4000'
                , '\t.END'
                ]
            )
        })
    })
    

    
})
