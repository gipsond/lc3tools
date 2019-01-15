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
        
        const saveAndAssemble = function (client, lines) {
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
            return saveAndAssemble(this.app.client,
                ['.ORIG x3000'
                , 'HALT'
                , '.END'
                ]
        )})

        it('an addition LC-3 program', function () {
            return saveAndAssemble(this.app.client,
                ['.ORIG x3000'
                , 'ADD R0, R0, R0'
                , 'HALT'
                , '.END'
                ]
        )})
    })
    

    
})
