import utils from '../utils'

describe('Simulator', function () {
    beforeEach(utils.beforeEachSim)
    afterEach(utils.afterEach)

    const firstMemLabelSelector = '#memview > div > div > table > tbody > tr:nth-child(1) > div:nth-child(3) > strong'

    it('initially shows memory beginning at 0x0200', function () {
        return this.app.client.click('#simulator-button')
            .then(() => { return this.app.client.getText(firstMemLabelSelector) })
            .then(firstMemLabel => {
                firstMemLabel.should.equal('0x0200')
            })
    })

    it('shows memory starting at the origin address of a program when a program is loaded', function () {
        return utils.saveAndAssemble(this.app.client,
                ['.ORIG x3000'
                , 'HALT'
                , '.END'
                ]
            )
            .then(() => { return this.app.client.click('#simulator-button') })
            .then(() => { return this.app.client.click('#open-file-button') })
            .then(() => { return this.app.client.getText(firstMemLabelSelector) })
            .then(firstMemLabel => {
                firstMemLabel.should.equal('0x3000')
            })
    })

    it('runs and displays results for the "Hello, World!" program', function () {
        this.timeout(5000);
        return utils.saveAndAssemble(this.app.client,
                ['.ORIG x3000'
                , 'LEA R0, GREETING'
                , 'PUTS'
                , 'HALT'
                , 'GREETING .STRINGZ "Hello, World!"'
                , '.END'
                ]
            )
            .then(() => { return this.app.client.click('#simulator-button') })
            .then(() => { return this.app.client.click('#open-file-button') })
            .then(() => { return this.app.client.click('#toggle-sim-button') })
            .then(() => { return this.app.client.getText('#console') })
            .then(consoleText => {
                consoleText.should.equal('\
Hello, World!\n\
\n\
--- Halting the LC-3 ---'
                )
            });
    })
})