#pragma once

class JPABaseEmitter;
class JPABaseParticle;

class JPAParticleCallBack {
public:
    virtual ~JPAParticleCallBack() = 0;
    virtual void execute(JPABaseEmitter*, JPABaseParticle*) {
    }

    virtual void draw(JPABaseEmitter*, JPABaseParticle*) {
    }
};
