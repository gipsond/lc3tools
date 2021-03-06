import Vue from 'vue'
import Router from 'vue-router'

Vue.use(Router)

export default new Router({
  routes: [
    {
      path: '/assembler',
      name: 'assembler',
      component: require('@/components/Assembler').default
    },
    {
      path: '/simulator',
      name: 'simulator',
      component: require('@/components/Simulator').default
    },
    {
      path: '*',
      redirect: '/assembler'
    }
  ]
})
