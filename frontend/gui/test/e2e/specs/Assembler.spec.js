import utils from '../utils'
import { doesNotThrow } from 'assert';

describe('Assembler', function () {
    beforeEach(utils.beforeEach)
    afterEach(utils.afterEach)

    it('shows an error when there is nothing to assemble', function () {
        return this.app.client.click('#build-button')
        .then(result => {
            return this.app.client.getHTML('#console', false)
        }).then(consoleContents => {
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

    it('successfully types the minimal LC-3 program', function () {
        return this.app.client.click('div.ace_content')
        .then(result => {
            return this.app.client.keys('.ORIG x3000') 
        }).then(result => {
            return this.app.client.keys('Enter')
        }).then(result => {
            return this.app.client.keys('HALT')
        }).then(result => {
            return this.app.client.keys('Enter')
        }).then(result => {
            return this.app.client.keys('.END')
        })
    })
})
