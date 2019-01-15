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

    it('successfully saves and assembles the minimal LC-3 program', function () {
        return this.app.client.click('div.ace_content')
        .then(() => { return this.app.client.keys('.ORIG x3000') })
        .then(() => { return this.app.client.keys('Enter')       })
        .then(() => { return this.app.client.keys('HALT')        })
        .then(() => { return this.app.client.keys('Enter')       })
        .then(() => { return this.app.client.keys('.END')        })
        .then(() => { return this.app.client.click('#save-button') })
        .then(() => { return this.app.client.click('#build-button')})
        .then(() => { return this.app.client.getHTML('#console', false) })
        .then(consoleContents => {
            consoleContents.should.equal('\
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
            )
        })
    })
})
