import utils from '../utils'

describe('Launch', function () {
    beforeEach(utils.beforeEach)
    afterEach(utils.afterEach)

    it('shows a single initial window', function () {
        return this.app.client.getWindowCount()
            .then(count => {
                count.should.equal(1)
            })
    })

    it('shows the proper application title', function () {
        return this.app.client.getTitle()
            .then(title => {
                title.should.equal('lc3tools')
            })
    })

})